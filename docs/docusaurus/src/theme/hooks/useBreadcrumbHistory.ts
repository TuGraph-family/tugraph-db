import { useState, useEffect } from 'react';
import { useLocation } from '@docusaurus/router';

export interface BreadcrumbItem {
    docId?: string;
    href: string;
    label: string;
    type: 'link' | 'html';
}

// Module-level variable shared across all components
let sharedBreadcrumbs: BreadcrumbItem[] = [];

// Extract version number from pathname
function getVersionFromPath(pathname: string): string | null {
    const match = pathname.match(/\/(\d+\.\d+(\.\d+)?)\//);
    return match ? match[1] : null;
}

// Check if history contains multiple different versions
function hasMultipleVersions(): boolean {
    const versions = sharedBreadcrumbs
        .map(item => getVersionFromPath(item.href))
        .filter(Boolean);

    const uniqueVersions = new Set(versions);
    return uniqueVersions.size > 1;
}

export const useBreadcrumb = () => {
    const location = useLocation();
    const [, forceUpdate] = useState(0);

    // Detect version change on route change
    useEffect(() => {
        if (hasMultipleVersions() && sharedBreadcrumbs.length > 0) {
            // Keep only the last item (current page)
            const lastItem = sharedBreadcrumbs[sharedBreadcrumbs.length - 1];
            sharedBreadcrumbs = [lastItem];
            forceUpdate(n => n + 1);
        }
    }, [location.pathname]);

    // Set breadcrumb history
    const setBreadcrumbHistory = (current: BreadcrumbItem) => {
        // Check if current page is already in history
        const existingIndex = sharedBreadcrumbs.findIndex(item => item.href === current.href);

        if (existingIndex !== -1) {
            // If exists, remove it and add to end
            const filtered = sharedBreadcrumbs.filter((_, i) => i !== existingIndex);
            sharedBreadcrumbs = [...filtered, current].slice(-3);
        } else {
            // If not exists, add to end (max 3 items)
            sharedBreadcrumbs = [...sharedBreadcrumbs, current].slice(-3);
        }

        // Check again after adding, if multiple versions then keep only last item
        if (hasMultipleVersions() && sharedBreadcrumbs.length > 0) {
            const lastItem = sharedBreadcrumbs[sharedBreadcrumbs.length - 1];
            sharedBreadcrumbs = [lastItem];
        }

        // Trigger update
        forceUpdate(n => n + 1);
    };

    return {
        breadcrumbs: sharedBreadcrumbs,
        setBreadcrumbHistory
    };
};